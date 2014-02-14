local str = "C'est Génial !"
for s in str:pgmatch('\\pL+', 'u') do
alert(s)
end
