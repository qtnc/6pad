<?php
function dlf ($s) {
global $dl;
if (isset($dl[$s])) return $dl[$s];
else return 0;
}
$dl = @unserialize(@file_get_contents('dl.txt'));
if (!$dl) $dl=array(
'fr11'=>366, 'en11'=>59, 'src11'=>45
);//
$dls = array(
'fr14' => '6pad1.4-fr.zip',
'en14' => '6pad1.4-en.zip',
'src14' => '6pad1.4-src.zip',
'fr131' => '6pad1.3.1-fr.zip',
'en131' => '6pad1.3.1-en.zip',
'src131' => '6pad1.3.1-src.zip',
'fr13' => '6pad1.3-fr.zip',
'en13' => '6pad1.3-en.zip',
'src13' => '6pad1.3-src.zip',
'fr121' => '6pad1.2.1-fr.zip',
'en121' => '6pad1.2.1-en.zip',
'src121' => '6pad1.2.1-src.zip',
'fr12' => '6pad1.2-fr.zip',
'en12' => '6pad1.2-en.zip',
'src12' => '6pad1.2-src.zip',
'fr11' => '6pad1.1-fr.zip',
'en11' => '6pad1.1-en.zip',
'src11' => '6pad1.1-src.zip',
'instable-fr' => 'https://svn.quentinc.net/6pad/6pad-fr.zip',
'instable-en' => 'https://svn.quentinc.net/6pad/6pad-en.zip'
);//end

if (isset($_GET['dl']) && isset($dls[$_GET['dl']])) {
$d = $_GET['dl'];
$fn = basename($dls[$d]);
if (isset($dl[$d])) $dl[$d]++;
else $dl[$d]=1;
@file_put_contents('dl.txt', @serialize($dl));
header('Content-Type:application/octet-stream');
header("Content-Disposition:attachment; filename=\"$fn\"");
readfile($dls[$d]);
exit();
}
$iTitle = $titrePage = $pageTitre = $pageTitle = '6pad';
require('../header.php');
require('../menu.php');
?>
<h2>6pad</h2>
<p><strong>L'éditeur de texte petit et malin</strong></p>
<p>6pad est un éditeur de texte pour windows destiné à remplacer l'archaïque bloc-notes.
IL a été dès le départ conçu pour conserver sa rapidité d'exécution et son accessibilité, tout en proposant de nouvelles fonctionnalités pas réellement innovantes mais utiles et pratiques.</p>
<h3>Fonctionnalités</h3>
<ul>
<li>100% accessible, car n'utilisant que les composants standards de windows</li>
<li>Petit et portable: moins de 500 Ko, pas d'installation nécessaire</li>
<li>Rapidité accrue, même à l'ouverture et l'édition de gros fichiers de plusieurs Mo (max. 1 Go)</li>
<li>Ouverture de fichiers multiples dans des onglets séparés</li>
<li>Gestion des 13 encodages les plus courants et des différents types de sauts de ligne</li>
<li>Recherche et remplacement par expressions régulières compatibles perl</li>
<li>Suivi de l'indentation automatique et touche origine intelligente</li>
<li>Protection contre la double-ouverture et les modifications concurrentes</li>
<li>Module complet et puissant de scripting en langage lua, permettant de personnaliser l'éditeur à vos exigences</li>
</ul>
<p>Puisque toutes les choses ont des inconvénients, les fonctionnalités suivantes <strong>ne sont PAS proposés</strong> et ne le <strong>seront certainement JAMAIS</strong> : </p>
<ul>
<li>Coloration syntaxique</li>
<li>Fonctionnalités d'impression</li>
<li>Fonctionnalités WYSIWYG</li>
</ul>
<h3>License</h3>
<p>Ce logiciel est sous license Creative Common BY/NC/SA. Cela signifie que vous êtes libre de l'utiliser, de le distribuer, de le modifier et de redistribuer une version modifiée, à condition que votre version modifiée conserve cette même license et que vous citiez son origine. En outre, son utilisation dans un but commercial sont interdites, sauf autorisation particulière.</p>
<p>JE décline toute responsabilité en cas de problème suite à l'utilisation de mon logiciel. Vous l'utilisez à vos risques et périles.</p>
<p>Si vous souhaitez contribuer, que ce soit pour le logiciel lui-même ou pour écrire des extensions en lua, vous pouvez <a href="/contact/">me contacter</a>.</p>
<h3>Téléchargement</h3>
<h4>Dernière version stable</h4>
<p>La dernière version stable est la version 1.4.1 du 25.10.2012. Pour une utilisation courante, c'est cette version que vous devez télécharger.</p>
<ul>
<li><a href="?dl=fr14">6pad 1.4.1 français</a> (fichier zip, ~250 Ko, <?php echo dlf('fr14'); ?> téléchargements depuis le 17.09.2012)</li>
<li><a href="?dl=en14">6pad 1.4.1 anglais</a> (fichier zip, ~250 Ko, <?php echo dlf('en14'); ?> téléchargements depuis le 17.09.2012)</li>
<li><a href="?dl=src14">Code source 1.4.1</a> (fichier zip, ~200 Ko, <?php echo dlf('src14'); ?> téléchargements depuis le 17.09.2012)</li>
</ul>
<h4>Anciennes versions</h4>
<ul>
<li><a href="?dl=fr131">6pad 1.3.1 français</a> (fichier zip, ~250 Ko, <?php echo dlf('fr131'); ?> téléchargements depuis le 03.02.2012)</li>
<li><a href="?dl=en131">6pad 1.3.1 anglais</a> (fichier zip, ~250 Ko, <?php echo dlf('en131'); ?> téléchargements depuis le 03.02.2012)</li>
<li><a href="?dl=src131">Code source 1.3.1</a> (fichier zip, ~200 Ko, <?php echo dlf('src131'); ?> téléchargements depuis le 03.02.2012)</li>
<li><a href="?dl=fr121">6pad 1.2.1 français</a> (fichier zip, ~250 Ko, <?php echo dlf('fr121'); ?> téléchargements depuis le 25.10.2011)</li>
<li><a href="?dl=en121">6pad 1.2.1 anglais</a> (fichier zip, ~250 Ko, <?php echo dlf('en121'); ?> téléchargements depuis le 25.10.2011)</li>
<li><a href="?dl=src121">Code source 1.2.1</a> (fichier zip, ~200 Ko, <?php echo dlf('src121'); ?> téléchargements depuis le 25.10.2011)</li>
<li><a href="?dl=fr11">6pad 1.1 français</a> (fichier zip, ~200 Ko, <?php echo dlf('fr11'); ?> téléchargements depuis le 19.03.2011)</li>
<li><a href="?dl=en11">6pad 1.1 anglais</a> (fichier zip, ~200 Ko, <?php echo dlf('en11'); ?> téléchargements depuis le 19.03.2011)</li>
<li><a href="?dl=src11">Code source 1.1</a> (fichier zip, ~200 Ko, <?php echo dlf('src11'); ?> téléchargements depuis le 18.05.2011)</li>
</ul>
<h4>Dernière version de développement</h4>
<p>La version en cours de développement est la 1.5. Cette version est vraiment la plus récente et contient les dernières nouveautés, mais elle n'est pas forcément très stable et contient probablement de nombreux bugs. Elle est réservée à ceux qui veulent profiter des dernières mises à jour, ceux qui veulent contribuer en testant le logiciel, ou simplement ceux qui ont le goût du risque.
<p>Pour accéder directement au répertoire SVN du projet: <a href="https://svn.quentinc.net/6pad/">https://svn.quentinc.net/6pad/</a></p>
<h3>Scripts d'extension</h3>
<p>6pad permettant le scripting en lua, ses fonctionnalités sont extensibles. Retrouvez dans cette rubrique une liste succinte de modules optionnels disponibles.</p>
<p>Si vous avez amélioré un de ces modules, ou si vous souhaitez en partager un que vous avez créé, <a href="/contact">contactez-moi par e-mail pour l'ajouter à la liste</a>. Nous ne pouvons que nous réjouir à l'apparition de nouveaux modules d'extensions, car il rendent 6pad encore plus puissant, unique et personnalisable.</p>
<p>Note 1: certains de ces modules ne sont peut-être pas sous license libre, en particulier si je n'en suis pas l'auteur.</p>
<p>Note 2: les modules qui ont été écrits pour des versions antérieures de 6pad peuvent ne plus fonctionner avec les nouvelles versions.</p>
<table>
<thead><tr>
<th>Nom</th>
<th>Auteur</th>
<th>Version de 6pad</th>
<th>Téléchargement</th>
<th>Description</th>
</tr></thead>
<tbody>
<tr><th>Autocomplete</th><td>QuentinC</td><td>1.3</td>
<td><a href="autocomplete.zip">autocomplete.zip</a></td>
<td>Prise en charge de l'autocompletion, avec possibilité de définir des dictionnaires différents selon le type de fichier ouvert. Dictionnaire non fourni.</td></tr>
<tr><th>Astyle #1</th><td>QuentinC</td><td>1.0</td>
<td><a href="astyle1.zip">astyle1.zip</a></td>
<td>Ce module utilise le logiciel astyle pour indenter automatiquement les codes sources C, C++, C# et java, très simplement et directement à le'enregistrement.</td></tr>
<tr><th>Astyle #2</th><td>QuentinC</td>
<td>1.0</td><td><a href="astyle2.zip">astyle2.zip</a></td>
<td>Ce module utilise le logiciel astyle pour indenter automatiquement les codes sources C, C++, C# et java, rapidement et à la demande, grâce à un nouvel élément de menu</td></tr>
<tr><th>Pybrace (alpha)</th><td>QuentinC</td><td>0.1 beta 6</td>
<td><a href="pybrace.zip">pybrace.zip</a></td>
<td>Ce module simplifie la programmation en python pour les utilisateurs non-voyants, pour qui l'indentation obligatoire de ce langage est un problème.
A l'ouverture d'un code source python, son indentation est convertie en structure classique à base d'accolades ressemblant au langage C ou java. A l'enregistrement, la structure en accolades est reconvertie en indentations normales.
LE tout est automatique, de façon à ce que ces transformations soient complètement transparentes.</td></tr>
</tbody></table>
<?php require('../footer.php'); ?>